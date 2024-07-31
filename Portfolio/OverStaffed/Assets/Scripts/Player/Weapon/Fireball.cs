using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.UI;

public class Fireball : MonoBehaviour
{
    [Header("----- Projectile Stats -----")]
    public int shotDmg;
    [SerializeField] int critChance;
    [SerializeField] float bulletLife;
    [SerializeField] float bulletRange;
    [SerializeField] int bulletSpeed;

    [Header("----- Components -----")]
    [SerializeField] Rigidbody rb;
    GameObject player;
    NewStaff staff;
    //[SerializeField] private Transform shootPos;
    [SerializeField] private LayerMask mask;
    [SerializeField] GameObject fireball;

    Vector3 playerPosition;
    Vector3 direction;

    // Start is called before the first frame update
    void Start()
    {
        Destroy(gameObject, bulletLife);
        player = GameObject.FindGameObjectWithTag("Player");
        playerPosition = player.transform.position;
        //rb.velocity = (player.transform.position - rb.position).normalized * bulletSpeed;//* Time.deltaTime;

        //direction = transform.parent.forward;
    }
    private void Update()
    {
        transform.RotateAround(gameManager.instance.playerCharacter.transform.position, Vector3.up * bulletRange, bulletSpeed * Time.deltaTime);
    }
}
