using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using static Skills;


public class UIManager : MonoBehaviour
{
    public static UIManager instance;

    [Header("----- UI Stuff -----")]
    public StoreButtons storeBtns;
    public GameObject HUD;
    public GameObject activeMenu;
    public GameObject mainMenu;
    public GameObject playerSelect;
    public GameObject pauseMenu;
    public GameObject loseMenu;
    public GameObject winMenu;
    public GameObject settingsMenu;
    public GameObject fademenu;
    public GameObject flashDamage;
    public GameObject levelSelectMenu;
    public GameObject storeMenu;
    public GameObject buyScreen;
    public GameObject sellScreen;
    public GameObject elementSelectMenu;
    public GameObject saveMenu;
    public GameObject creditsMenu;
    public GameObject quitCheckMenu;
    public GameObject beginLetter;
    public GameObject endLetter;
    public GameObject interactTextGameObject;
    public GameObject gamePlayRecap;
    public TextMeshProUGUI interactText;
    public TextMeshProUGUI totalLevelsCompleted;
    public TextMeshProUGUI totalenemiesDefeated;
    public TextMeshProUGUI expGained;
    public Animator creditSlide;
    public Animator levelComplete;


    [Header("----- HUD Pieces -----")]
    public GameObject highJumpIndicator;
    public GameObject dashIndicator;
    public GameObject blinkIndicator;
    public GameObject specialIndicator;
    public GameObject expIndicator;
    public GameObject healthIndicator;
    public GameObject burnIndicator;
    public GameObject poisonIndicator;
    public GameObject freezeIndicator;
    public GameObject levelCompleteText;
    public GameObject potionsIndicator;

    public Image playerHealthBar;
    public TextMeshProUGUI levelText;
    public TextMeshProUGUI hpText;
    public TextMeshProUGUI potionText;
    public TextMeshProUGUI expText;
    public TextMeshProUGUI healthText;
    public TextMeshProUGUI speedText;
    public TextMeshProUGUI attackText;
    public TextMeshProUGUI storeCurrency;
    [SerializeField] Image UtCharge;

    [Header("-----Fade Stuff-----")]
    public int fadeSpeed;
    public Animator fadeScreen;
    public bool isFading;


    [Header("-----Tutorial Stuff-----")]
    public GameObject tut1;
    public GameObject tut2;
    public GameObject tut3;
    public GameObject tut4;

    [Header("-----Misc Stuff-----")]
    public Toggle invert;
    public Slider mouseSen;

    public Image ability1; //Hi-Jump
    public Image ability2; //Dash
    public Image ability3; //Blink
    public Sprite[] spriteArray;
    public Image elementSprite;

    private gameManager gameManager;
    private NewStaff.Element playerElement;

    private LevelManager levelManager;

    private Skills playerSkills;
    float waitTime;


    private void Awake()
    {
        if(UIManager.instance != null)
        {
            DestroyImmediate(this.gameObject);
        }
        else
        {
            instance = this;
        }
       
    }

    private void Start()
    {
        fileManager.firstLoad();
       
        activeMenu = mainMenu;
        //playerElement = gameManager.playerElement;
        gameManager = gameManager.instance;
        levelManager = LevelManager.instance;
        playerSkills = gameManager.playerSkills;
        //fileManager.save();
        fileManager.load();
    }

    void Update()
    {

         if (Input.GetButtonDown("Cancel") && activeMenu == null && !isFading)
         {

             AudioManager.instance.MenuTransition();
             activeMenu = pauseMenu;
             ShowActiveMenu();
             gameManager.PauseState();
             flashDamage.SetActive(false);

         }

        if(gameManager.playerCharacter != null)
        {
            AbilityCoolDown();

            SetElement();
            SetElementIcon();
            UpdatePotionCount();
        }
        if (fadeScreen.GetCurrentAnimatorStateInfo(0).IsName("CrossFadeEnd") && fadeScreen.GetCurrentAnimatorStateInfo(0).normalizedTime >= 1.0f)
        {
            isFading = false;
        }
        if (creditsMenu.activeSelf)
        {
            ExitCreditScreen();
           
        }

        PlayerProgression();

    }
    public void YouLose()
    {
        AudioManager.instance.MenuTransition();
        gameManager.PauseState();
        activeMenu = loseMenu;
        ShowActiveMenu();
        totalLevelsCompleted.text = ($"{levelManager.currentLevel - 1}");
        totalenemiesDefeated.text = ($"{levelManager.totalEnemiesDefeated}");
        gamePlayRecap.SetActive(true);
    }

    public void YouWin()
    {
        AudioManager.instance.MenuTransition();
        gameManager.PauseState();
        activeMenu = winMenu;
        ShowActiveMenu();

    }

    public void ShowEndLetter()
    {
        AudioManager.instance.MenuTransition();
        gameManager.PauseState();
        activeMenu = endLetter;
        ShowActiveMenu();
    }

    public void ShowPostRunStats()
    {
        AudioManager.instance.MenuTransition();
        gameManager.PauseState();
        gamePlayRecap.SetActive(true);
        totalLevelsCompleted.text = ($"{levelManager.currentLevel - 1}");
        totalenemiesDefeated.text = ($"{levelManager.totalEnemiesDefeated}");

    }
    public void ShowActiveMenu() //shows active menu if there is one.
    {
        if (activeMenu != null)
        {
            activeMenu.SetActive(true);
        }
    }

    public void HideActiveMenu() //hides active menu and sets it to null
    {
        if (activeMenu != null)
        {
            activeMenu.SetActive(false);
            activeMenu = null;
        }
    }

    public void ExitCreditScreen()
    {
        if (creditSlide.GetCurrentAnimatorStateInfo(0).normalizedTime >= 1.0f  || Input.anyKeyDown)
        {
            creditsMenu.SetActive(false);
            if(levelManager.currentLevel == 22)
            {
                YouWin();
            }
        }
    }

    public void ShowDamage()
    {
        StartCoroutine(FlashRed());
    }

    IEnumerator FlashRed()
    {
        flashDamage.SetActive(true);
        yield return new WaitForSeconds(0.3f);
        flashDamage.SetActive(false);
    }

    public void SetElement()
    {
        playerElement = gameManager.playerController.playerElement;
    }

    //displays the correct element based on character type
    public void SetElementIcon()
    {
        //Debug.Log("Set ELement");
        //Debug.Log(playerScript.GetWeapon());
        elementSprite.sprite = spriteArray[(int) playerElement];
        switch (playerElement)
        {
            case NewStaff.Element.Fire:
                UtCharge.color = Color.yellow;
                break;
            case NewStaff.Element.Water:
                UtCharge.color = Color.blue;
                break;
            case NewStaff.Element.Earth:
                UtCharge.color = Color.green;
                break;
        }
    }

    //update level counter in UI
    public void UpdateLevelCount()
    {
        int level = levelManager.currentLevel;
        levelText.text = level.ToString("F0");
    }

    public void ShowLevelCompleteText()
    {
        levelCompleteText.SetActive(true);
    }
    public void StopLevelCompleteText()
    {
        levelCompleteText.SetActive(false);
    }
    public void UpdateExp()
    {
        int exp = Stats.Exp;
        expText.text = exp.ToString("F0");
        storeCurrency.text = exp.ToString("F0");
        expGained.text = exp.ToString("F0");
    }

    public void UpdateHealthAmount()
    {
        int health = gameManager.instance.playerController.totalHP;
        healthText.text = health.ToString("F0");

    }
    public void UpdateSpeedAmount()
    {
        float speed = gameManager.instance.playerController.playerSpeed;
        speedText.text = speed.ToString("F0");
    }
    public void UpdateAttackAmount()
    {
        int attack = gameManager.instance.playerController.playerDamage;
        attackText.text = attack.ToString("F0");
    }
    public void UpdatePotionCount()
    {
        int potionCount = gameManager.instance.playerController.potionsAvailable;
        potionText.text = potionCount.ToString("F0");
    }

    public void AbilityCoolDown()
    {
        if (playerSkills.isJumpCooldown())
        {
            ability1.gameObject.SetActive(true);

            waitTime = playerSkills.getCooldown(skill.HiJump);
            ability1.fillAmount -= 1.0f / waitTime * Time.deltaTime;
        }
        if (playerSkills.isDashCooldown())
        {
            ability2.gameObject.SetActive(true);

            waitTime = playerSkills.getCooldown(skill.Dash);
            ability2.fillAmount -= 1.0f / waitTime * Time.deltaTime;
        }

        if (playerSkills.isBlinkCooldown())
        {
            ability3.gameObject.SetActive(true);

            waitTime = playerSkills.getCooldown(skill.Blink);
            ability3.fillAmount -= 1.0f / waitTime * Time.deltaTime;
        }

        if (!playerSkills.isJumpCooldown())
        {
            ability1.gameObject.SetActive(false);
            ability1.fillAmount = 1;
        }

        if (!playerSkills.isDashCooldown())
        {
            ability2.gameObject.SetActive(false);
            ability2.fillAmount = 1;
        }

        if (!playerSkills.isBlinkCooldown())
        {
            ability3.gameObject.SetActive(false);
            ability3.fillAmount = 1;
        }
    }

    //Animation for fade screen
    public IEnumerator FadeScreen()
    {
        isFading = true;
        fadeScreen.SetTrigger("StartFade");
        yield return new WaitForSeconds(fadeSpeed);
        LevelManager.instance.LoadNextLevel();
        
    }

    public void UpdateUtCharge(float amount)
    {
        UtCharge.fillAmount = amount;
    }

    public void SetPlayerVariables()
    {
        Debug.Log(gameManager.instance.playerSkills);
        playerSkills = gameManager.instance.playerSkills;
        playerElement = gameManager.instance.playerElement;
    }

    public void UpdateInteractText(int interactionScenario, string textToShow = "")
    {
        switch(interactionScenario)
        {
            case 0: //nothing found
                {
                    interactText.text = textToShow;
                    interactTextGameObject.SetActive(false);
                    break;
                }
            case 1:
                {
                    interactText.text = textToShow;
                    interactTextGameObject.SetActive(true);
                    break;
                }
        }
    }

    //highJumpIndicator;
    //dashIndicator;
    //blinkIndicator;
    //specialIndicator;
    //expIndicator;
    //healthIndicator;
    //enemiesRemainingIndicator;

    public void PlayerProgression()
    {
        if (levelManager.currentLevel >= 2) //high jump
        {
            highJumpIndicator.SetActive(true);
        }
        else
        {
            highJumpIndicator.SetActive(false);
        }

        if (levelManager.currentLevel >= 3) //dash
        {
            dashIndicator.SetActive(true);
        }
        else
        {
            dashIndicator.SetActive(false);
        }

        if (levelManager.currentLevel >= 4) //blink
        {
            blinkIndicator.SetActive(true);
        }
        else
        {
            blinkIndicator.SetActive(false);
        }

        if (levelManager.currentLevel >= 5) //special, exp, enemies
        {
            specialIndicator.SetActive(true);
            expIndicator.SetActive(true);
            healthIndicator.SetActive(true);
            potionsIndicator.SetActive(true);
            //enemiesRemainingIndicator.SetActive(true);
        }
        else
        {
            specialIndicator.SetActive(false);
            expIndicator.SetActive(false);
            healthIndicator.SetActive(false);
            potionsIndicator.SetActive(false);
           // enemiesRemainingIndicator.SetActive(false);
        }
    }
}
