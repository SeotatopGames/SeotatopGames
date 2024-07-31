using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEditor;
using UnityEngine;
using UnityEngine.SceneManagement;

public class PlayerSelect : MonoBehaviour
{
    [SerializeField] GameObject player;
    [SerializeField] GameObject levelManager;
    [SerializeField] GameObject UI;

    PlayerController playerController;

    private void Start()
    {
        if(Cursor.visible == false)
        {
            Cursor.visible = true;
            Cursor.lockState = CursorLockMode.Confined;
        }
        Cursor.visible = true;
        Cursor.lockState = CursorLockMode.Confined;
    }
    public void SelectedFire()
    {
        PrePlayerElementSetup();
        playerController.playerElement = NewStaff.Element.Fire;
        PostPlayerElementSetup();
        playerController.ChangeBaseStats(NewStaff.Element.Fire);
    }

    public void SelectedWater()
    {
        PrePlayerElementSetup();
        playerController.playerElement = NewStaff.Element.Water;
        PostPlayerElementSetup();
        playerController.ChangeBaseStats(NewStaff.Element.Water);
    }

    public void SelectedEarth()
    {
        PrePlayerElementSetup();
        playerController.playerElement = NewStaff.Element.Earth;
        PostPlayerElementSetup();
        playerController.ChangeBaseStats(NewStaff.Element.Earth);
    }

    public void PrePlayerElementSetup() //must happen before player element setup occurs
    {
        DestroyImmediate(Camera.main.gameObject);
        Instantiate(player);
        player = GameObject.FindGameObjectWithTag("Player");
        //Debug.Log("Player Spawned");
        playerController = player.GetComponent<PlayerController>();
        //Debug.Log("Player Controller Set");
    }

    public void PostPlayerElementSetup() //must happen after player element setup occurs
    {
        //Debug.Log("Player Element Set");
        SceneManager.LoadScene("Reception");
    }

    public void BackMainMenu()
    {
        SceneManager.LoadScene("Main Menu");
    }
}
